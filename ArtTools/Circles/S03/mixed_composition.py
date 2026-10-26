"""Offline candidate pose data for review only. No Skyrim/gameplay implementation."""
from pathlib import Path
import json
import math

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT/'generator/magician_mixed.candidate.json'


def read_config():
    c = json.loads(CONFIG.read_text(encoding='utf-8'))
    assert [r['target_scale'] for r in c['circles']] == [1, 1.35, .70, 1.55, 1.05, .65, 1.40, .90]
    assert c['thresholds_mp'] == [100, 300, 600, 1200, 2000, 3000, 4000, 5000]
    assert c['reference_timing']['tier_arrival_seconds'] == [q/500 for q in c['thresholds_mp']]
    assert c['compression_candidate']['end_seconds'] == 10
    return c


def smootherstep(u):
    u = min(1.0, max(0.0, u))
    return u*u*u*(10+u*(-15+6*u))


def spacing_at(config, t):
    c = config['compression_candidate']
    u = (t-c['start_seconds'])/(c['end_seconds']-c['start_seconds'])
    return c['spacing_y_start']+(c['spacing_y_end']-c['spacing_y_start'])*smootherstep(u)


def spacing_velocity(config, t):
    c = config['compression_candidate']
    duration = c['end_seconds']-c['start_seconds']
    u = (t-c['start_seconds'])/duration
    if not 0 < u < 1:
        return 0.0
    return (c['spacing_y_end']-c['spacing_y_start'])*30*u*u*(1-u)*(1-u)/duration


def pose(config, t, full_size_inspection=False):
    times = config['reference_timing']['tier_arrival_seconds']
    growth = config['appearance_candidate']
    instances = []
    spacing = spacing_at(config, t)
    for i, c in enumerate(config['circles']):
        visible = t >= times[i]
        factor = 1.0
        if visible and i and not full_size_inspection:
            factor = growth['initial_fraction_of_target_scale']+(1-growth['initial_fraction_of_target_scale'])*smootherstep((t-times[i])/growth['duration_seconds'])
        instances.append({**c, 'visible': visible, 'scale': c['target_scale']*factor,
                          'translation': [0, i*spacing, 0] if visible else None,
                          'rotation_y_degrees': 0, 'velocity_y': i*spacing_velocity(config, t) if visible else None})
    return {'time_seconds': t, 'visible_count': sum(c['visible'] for c in instances),
            'spacing_y': spacing, 'spacing_velocity_y': spacing_velocity(config, t), 'instances': instances}


def validate(config):
    values = [spacing_at(config, i/100) for i in range(1101)]
    assert all(a >= b-1e-10 for a, b in zip(values, values[1:]))
    assert spacing_at(config, 0) == spacing_at(config, 1.2) == 96
    assert spacing_at(config, 10) == spacing_at(config, 11) == 60
    assert spacing_velocity(config, 1.2) == spacing_velocity(config, 10) == 0
    for i, t in enumerate(config['reference_timing']['tier_arrival_seconds']):
        assert pose(config, t)['visible_count'] == i+1
        assert pose(config, t-1e-7)['visible_count'] == i
    for t in [i/60 for i in range(661)]:
        p = pose(config, t)
        for i, c in enumerate(p['instances']):
            assert math.isfinite(c['scale']) and c['scale'] > 0
            if c['visible']:
                assert all(map(math.isfinite, c['translation']))
                if i == 0:
                    assert c['translation'] == [0, 0, 0] and c['scale'] == 1
                elif t >= config['reference_timing']['tier_arrival_seconds'][i]+.24:
                    assert abs(c['scale']-c['target_scale']) < 1e-12
    # Compression has no per-tier reset: position and velocity are continuous across all thresholds.
    for t in config['reference_timing']['tier_arrival_seconds'][2:]:
        assert abs(spacing_at(config, t+1e-6)-spacing_at(config, t-1e-6)) < 2e-5
        assert abs(spacing_velocity(config, t+1e-6)-spacing_velocity(config, t-1e-6)) < 2e-5
    return {'status': 'STATIC QA PASS', 'monotone_spacing': True, 'c1_fixed': True,
            'strict_visibility': True, 'existing_size_identity': True,
            'continuous_compression_position_velocity_at_tiers': True,
            'compression_complete_seconds': 10.0, 'c8_full_size_seconds': 10.24,
            'runtime_dynamic_retarget': 'RUNTIME UNVERIFIED; not implemented'}


def write_values(config):
    tiers = []
    for i, t in enumerate(config['reference_timing']['tier_arrival_seconds']):
        tiers.append({'tier': i+1, 'mp': config['thresholds_mp'][i], **pose(config, t, True)})
    path = ROOT/'reports/MAGICIAN_TIER_PREVIEW_VALUES.json'
    path.write_text(json.dumps({'schema': config['schema'], 'status': config['status'],
                                'interpretation': 'Full-size structural inspection at tier arrival spacing; growth omitted',
                                'tiers': tiers}, indent=2), encoding='utf-8')
    return tiers
